(: Name: typeswitchhq2 :)
(: Description: Simple test for evaluation of atomic value (decimal) and various data types for case evaluation :)

typeswitch (5.1)
 case $i as xs:decimal
    return <wrap>test passed - 5.1 is a decimal type</wrap>
 case $i as xs:float
    return <wrap>test failed</wrap> 
 case $i as xs:integer
   return <wrap>test failed</wrap> 
 case $i as xs:double
   return <wrap>test failed</wrap>
 default
    return <wrap>test failed</wrap>