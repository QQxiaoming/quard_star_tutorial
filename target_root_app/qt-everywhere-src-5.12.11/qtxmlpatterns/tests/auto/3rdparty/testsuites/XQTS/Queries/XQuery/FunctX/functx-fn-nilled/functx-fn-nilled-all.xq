(:**************************************************************:)
(: Test: functx-fn-nilled-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <root xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <child>12</child>
  <child xsi:nil="true"></child>
  <child></child>
  <child/>
  <child xsi:nil="false"></child>
</root>

return (nilled($in-xml//child[1]), nilled($in-xml//child[3]), nilled($in-xml//child[4]), nilled($in-xml//child[5]))