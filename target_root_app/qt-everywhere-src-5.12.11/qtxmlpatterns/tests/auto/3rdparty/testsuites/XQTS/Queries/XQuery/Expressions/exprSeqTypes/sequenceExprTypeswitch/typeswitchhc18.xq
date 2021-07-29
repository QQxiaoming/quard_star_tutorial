(: Name: typeswitchhq18 :)
(: Description: Simple test for evaluation of atomic value (decimal) and return clause conatining an arithmetic expression with decimals:)

typeswitch (5.1)
 case $i as xs:decimal
    return xs:decimal(1.1 + 3.1)
 case $i as xs:float
    return <wrap>test failed</wrap> 
 case $i as xs:integer
   return <wrap>test failed</wrap> 
 case $i as xs:double
   return <wrap>test failed</wrap>
 default
    return <wrap>test failed</wrap>