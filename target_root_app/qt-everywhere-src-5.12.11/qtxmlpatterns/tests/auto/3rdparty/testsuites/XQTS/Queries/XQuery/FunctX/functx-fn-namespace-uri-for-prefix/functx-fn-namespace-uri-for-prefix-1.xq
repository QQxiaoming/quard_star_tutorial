(:**************************************************************:)
(: Test: functx-fn-namespace-uri-for-prefix-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
declare namespace pre = "http://datypic.com/pre";
declare namespace unpre = "http://datypic.com/unpre";

let $in-xml := <noNamespace>
     <pre:prefixed xmlns="http://datypic.com/unpre"
              xmlns:pre="http://datypic.com/pre">
       <unprefixed pre:prefAttr="a" noNSAttr="b">123</unprefixed>
     </pre:prefixed>
   </noNamespace>
return (namespace-uri-for-prefix(
  '', $in-xml))
