(:**************************************************************:)
(: Test: functx-fn-in-scope-prefixes-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <in-xml xmlns="http://datypic.com/prod"
          xmlns:prod2="http://datypic.com/prod2">
            xyz</in-xml>
return ()