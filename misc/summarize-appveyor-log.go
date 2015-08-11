//summarize MSVC errors from an appveyor log
// compile with 'go build summarize-appveyor-log.go'
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"regexp"
	"sort"
	"strings"
)

const (
	headerKey = "Authorization"
	headerVal = "Bearer %s"
	projUrl = "https://ci.appveyor.com/api/projects/mpictor/stepcode"
	//"https://ci.appveyor.com/api/buildjobs/2rjxdv1rnb8jcg8y/log"
	logUrl = "https://ci.appveyor.com/api/buildjobs/%s/log"
)

//uses stdin and stdout
func main() {
	rawlog, build, err := getLog()
	if err != nil {
		fmt.Fprintf(os.Stderr, "ERROR: %s\n", err)
		return
	}
	defer rawlog.Close()
	log := unwrap(rawlog)
	warns, errs := countMessages(log)
	fi, err := os.Create(fmt.Sprintf("appveyor-%d.smy", build))
	if err != nil {
		fmt.Fprintf(os.Stderr, "ERROR: %s\n", err)
		return
	}
	printMessages("error", errs, fi)
	printMessages("warning", warns, fi)

	fmt.Printf("done\n")

}

/* categorizes warnings and errors based upon the MSVC message number (i.e. C4244)
 * the regex will match lines like
[ 00:03:42] c:\projects\stepcode\src\base\sc_benchmark.h(45): warning C4251: 'benchmark::descr' : class 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>' needs to have dll-interface to be used by clients of class 'benchmark' [C:\projects\STEPcode\build\src\base\base.vcxproj]
[00:03:48] C:\projects\STEPcode\src\base\sc_benchmark.cc(61): warning C4244: '=' : conversion from 'SIZE_T' to 'long', possible loss of data [C:\projects\STEPcode\build\src\base\base.vcxproj]*
*/
func countMessages(log []string) (warns, errs map[string][]string) {
	warns = make(map[string][]string)
	errs = make(map[string][]string)
	tstamp := `\[\d\d:\d\d:\d\d\] `
	fname := " *(.*)"            // $1
	fline := `(?:\((\d+)\)| ): ` // $2 - either line number in parenthesis or a space, followed by a colon
	msgNr := `([A-Z]+\d+): `     // $3 - C4251, LNK2005, etc
	msgTxt := `([^\[]*) `        // $4
	tail := `\[[^\[\]]*\]`
	warnRe := regexp.MustCompile(tstamp + fname + fline + `warning ` + msgNr + msgTxt + tail)
	errRe := regexp.MustCompile(tstamp + fname + fline + `(?:fatal )?error ` + msgNr + msgTxt + tail)
	for _, line := range log {
		if warnRe.MatchString(line) {
			key := warnRe.ReplaceAllString(line, "$3")
			path := strings.ToLower(warnRe.ReplaceAllString(line, "$1:$2"))
			arr := warns[key]
			if arr == nil {
				arr = make([]string, 5)
				//detailed text as first string in array
				text := warnRe.ReplaceAllString(line, "$4")
				arr[0] = fmt.Sprintf("%s", text)
			}
			//eliminate duplicates
			match := false
			for _, l := range arr {
				if l == path {
					match = true
				}
			}
			if !match {
				warns[key] = append(arr, path)
			}
		} else if errRe.MatchString(line) {
			key := errRe.ReplaceAllString(line, "$3")
			path := strings.ToLower(errRe.ReplaceAllString(line, "$1:$2"))
			arr := errs[key]
			if arr == nil {
				arr = make([]string, 5)
				//detailed text as first string in array
				text := errRe.ReplaceAllString(line, "$4")
				arr[0] = fmt.Sprintf("%s", text)
			}
			//eliminate duplicates
			match := false
			for _, l := range arr {
				if l == path {
					match = true
				}
			}
			if !match {
				errs[key] = append(arr, path)
			}
		}
	}
	return
}

func printMessages(typ string, m map[string][]string, w io.Writer) {
	//sort keys
	keys := make([]string, 0, len(m))
	for key := range m {
		keys = append(keys, key)
	}
	sort.Strings(keys)
	for _, k := range keys {
		for i, l := range m[k] {
			//first string is an example,  not a location
			if i == 0 {
				fmt.Fprintf(w, "%s %s (i.e. \"%s\")\n", typ, k, l)
			} else if len(l) > 1 { //not sure where blank lines are coming from...
				fmt.Fprintf(w, "  >> %s\n", l)
			}
		}
	}
}

//
func unwrap(r io.Reader) (log []string) {
	startNewLine := true
	unwrapScanner := bufio.NewScanner(r)
	var lineOut string
	for unwrapScanner.Scan() {
		lastNewline := startNewLine
		lineIn := unwrapScanner.Text()
		startNewLine = (len(lineIn) < 240) || strings.HasSuffix(lineIn, "vcxproj]")
		if !lastNewline {
			lineOut += lineIn[11:]
		} else {
			lineOut = lineIn
		}
		if startNewLine {
			log = append(log, lineOut)
			lineOut = ""
		}
	}
	if len(lineOut) > 0 {
		log = append(log, lineOut)
	}
	if err := unwrapScanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "Error reading appveyor log:", err)
	}
	return
}

//http://json2struct.mervine.net/
type AppVeyorBuild struct {
	Build struct {
		BuildNumber int `json:"buildNumber"`
		Jobs        []struct {
			JobID string `json:"jobId"`
		} `json:"jobs"`
	} `json:"build"`
}

func getLog() (log io.ReadCloser, build int, err error) {
	client := &http.Client{}
	req, err := http.NewRequest("GET", projUrl, nil)
	if err != nil {
		return
	}
	apikey := os.Getenv("APPVEYOR_API_KEY")
	//api key isn't necessary for read-only queries on public projects
	//if len(apikey) < 1 {
	//	fmt.Printf("Env var APPVEYOR_API_KEY is not set.")
	//}
	req.Header.Add(headerKey, fmt.Sprintf(headerVal,apikey))
	resp, err := client.Do(req)
	if err != nil {
		return
	}

	build, job := decode(resp.Body)
	fmt.Printf("build #%d, jobId %s\n", build, job)
	resp, err = http.Get(fmt.Sprintf(logUrl, job))
	if err != nil {
		return
	}
	logName := fmt.Sprintf("appveyor-%d.log", build)
	fi, err := os.Create(logName)
	if err != nil {
		return
	}
	_, err = io.Copy(fi, resp.Body)
	if err != nil {
		return
	}
	log, err = os.Open(logName)
	if err != nil {
		log = nil
	}
	return
}

func decode(r io.Reader) (num int, job string) {
	dec := json.NewDecoder(r)
	var av AppVeyorBuild
	err := dec.Decode(&av)
	if err != io.EOF && err != nil {
		fmt.Printf("err %s\n", err)
		return
	}
	if len(av.Build.Jobs) != 1 {
		return
	}
	num = av.Build.BuildNumber
	job = av.Build.Jobs[0].JobID
	return
}

// kate: indent-width 8; space-indent off; replace-tabs off; replace-tabs-save off; replace-trailing-space-save on; remove-trailing-space on; tab-intent on; tab-width 8; show-tabs off;
