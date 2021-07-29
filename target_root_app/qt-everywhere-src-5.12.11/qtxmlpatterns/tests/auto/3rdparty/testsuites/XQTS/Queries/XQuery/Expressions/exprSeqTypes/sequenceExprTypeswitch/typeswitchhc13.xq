(: Name: typeswitchhq13 :)
(: Description: Simple test for typeswitch with operand expression (if Expression) evaluating to an integer. :)

typeswitch(if (1 lt 2) then 3 else 4.5E4)
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:integer
    return <wrap>test passed - "(1 lt 2) then 3 else 4.5E4" should evaluate to an integer</wrap>
 default
   return <wrap>test failed</wrap>
