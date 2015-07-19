//summarize MSVC errors from an appveyor log
// compile with 'go build summarize-appveyor-log.go'
package main

import (
	"bufio"
	"fmt"
	"os"
	"regexp"
	"strings"
)

//uses stdin and stdout
func main() {
	log := unwrap()
	warns, errs := countMessages(log)
	printMessages("error", errs)
	printMessages("warning", warns)
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
	fname := " *(.*)"
	fline := `\((\d+)\): `
	msgNr := `([A-Z]\d+): `
	msgTxt := `([^\[]*) `
	tail := `\[[^\[\]]*\]`
	warnRe := regexp.MustCompile(tstamp + fname + fline + `warning ` + msgNr + msgTxt + tail)
	errRe := regexp.MustCompile(tstamp + fname + fline + `(?:fatal )?error ` + msgNr + msgTxt + tail)
	//reScanner := bufio.NewScanner(strings.NewReader(...log))
	//for reScanner.Scan() {
		//line := reScanner.Text()
	for _,line := range log {
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

func printMessages(typ string, m map[string][]string) {
	for k, v := range m {
		for i, l := range v {
			//first string is an example,  not a location
			if i == 0 {
				fmt.Printf("%s %s (i.e. \"%s\")\n", typ, k, l)
			} else if len(l) > 1 { //not sure where blank lines are coming from...
				fmt.Printf("  >> %s\n", l)
			}
		}
	}
}

//
func unwrap() (log []string) {
	startNewLine := true
	unwrapScanner := bufio.NewScanner(os.Stdin)
	var lineOut string
	for unwrapScanner.Scan() {
		lastNewline := startNewLine
		lineIn := unwrapScanner.Text()
		startNewLine = (len(lineIn) < 240) || strings.HasSuffix(lineIn,"vcxproj]")
		if !lastNewline {
			lineOut += lineIn[11:]
		} else {
			lineOut = lineIn
		}
		if startNewLine {
			log = append(log,lineOut)
			lineOut = ""
			//log += fmt.Sprintf("\n")
		}
	}
	if len(lineOut) > 0 {
		log = append(log,lineOut)
	}
	if err := unwrapScanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "Error reading appveyor log:", err)
	}
	return
}

// kate: indent-width 8; space-indent off; replace-tabs off; replace-tabs-save off; replace-trailing-space-save on; remove-trailing-space on; tab-intent on; tab-width 8; show-tabs off;
