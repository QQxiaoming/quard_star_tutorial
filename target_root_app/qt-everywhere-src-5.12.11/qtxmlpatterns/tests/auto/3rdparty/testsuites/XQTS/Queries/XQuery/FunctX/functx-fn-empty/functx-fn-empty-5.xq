(:**************************************************************:)
(: Test: functx-fn-empty-5                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <in-xml>
     <a></a>
     <b/>
     <c>xyz</c>
   </in-xml>
return (empty($in-xml/b))
