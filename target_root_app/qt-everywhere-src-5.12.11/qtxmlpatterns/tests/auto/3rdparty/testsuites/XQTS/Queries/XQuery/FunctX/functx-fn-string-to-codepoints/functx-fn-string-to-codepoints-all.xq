(:**************************************************************:)
(: Test: functx-fn-string-to-codepoints-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(string-to-codepoints('abc'), string-to-codepoints('a'), string-to-codepoints(''))