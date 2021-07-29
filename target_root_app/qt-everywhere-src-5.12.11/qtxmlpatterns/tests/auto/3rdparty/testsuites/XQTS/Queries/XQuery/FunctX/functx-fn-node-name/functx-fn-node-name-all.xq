(:**************************************************************:)
(: Test: functx-fn-node-name-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
declare namespace pre2 = "http://datypic.com/pre";
declare namespace unpre2 = "http://datypic.com/unpre";

let $in-xml := <noNamespace>
  <pre:prefixed xmlns="http://datypic.com/unpre"
           xmlns:pre="http://datypic.com/pre">
    <unprefixed pre:prefAttr="a" noNSAttr="b">123</unprefixed>
  </pre:prefixed>
</noNamespace>
return (node-name($in-xml), node-name($in-xml/pre2:prefixed), node-name($in-xml//unpre2:unprefixed), node-name($in-xml//@pre2:prefAttr), node-name($in-xml//@noNSAttr))