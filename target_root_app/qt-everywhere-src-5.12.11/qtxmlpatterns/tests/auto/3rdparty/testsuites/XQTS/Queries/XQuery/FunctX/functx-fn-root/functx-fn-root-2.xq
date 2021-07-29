(:**************************************************************:)
(: Test: functx-fn-root-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <a><x>123</x></a>
return (root($in-xml/x))
