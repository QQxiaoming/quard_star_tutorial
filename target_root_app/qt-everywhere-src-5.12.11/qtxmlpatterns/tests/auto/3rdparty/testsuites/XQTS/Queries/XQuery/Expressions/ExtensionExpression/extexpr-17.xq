(: Name: extexpr-17 :)
(: Description: An extension expression for which the pragma is ignored and default expression is a "typeswitch" expression :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
{typeswitch ("A String")
  case $i as xs:decimal
    return "test failed" 
  case $i as xs:integer
   return "test failed" 
  case $i as xs:string
    return "test passed"
  default
   return "test failed" }
