(:*******************************************************:)
(: Test: Steps-leading-lone-slash-14                     :)
(: Written by: Jim Melton                                :)
(: Date: 2009-10-01T14:00:00-07:00                       :)
(: Purpose: Verify xgc:leading-lone-slash implementation :)
(:   This expression is a valid path expression          :)
(:      /max(a)                                          :)
(:*******************************************************:)
declare variable $var := document {<a>123</a>};
$var[/max(a)]