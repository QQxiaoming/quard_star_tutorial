(:**************************************************************:)
(: Test: functx-fn-prefix-from-QName-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
declare namespace pre2 = "http://datypic.com/pre";
declare namespace unpre = "http://datypic.com/unpre";

let $in-xml := <noNamespace>
  <pre:prefixed xmlns="http://datypic.com/unpre"
           xmlns:pre="http://datypic.com/pre">
    <unprefixed pre:prefAttr="a" noNSAttr="b">123</unprefixed>
  </pre:prefixed>
</noNamespace>
return (prefix-from-QName(
  node-name($in-xml)), prefix-from-QName(
  node-name($in-xml//pre2:prefixed)), prefix-from-QName(
  node-name($in-xml//unpre:unprefixed)), prefix-from-QName(
  node-name($in-xml//@pre2:prefAttr)), prefix-from-QName(
  node-name($in-xml//@noNSAttr)), prefix-from-QName(
  node-name(<pre2:new>xyz</pre2:new>)))