(:**************************************************************:)
(: Test: functx-fn-replace-16                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(replace('abc123', '([a-z])', '$1x'))
