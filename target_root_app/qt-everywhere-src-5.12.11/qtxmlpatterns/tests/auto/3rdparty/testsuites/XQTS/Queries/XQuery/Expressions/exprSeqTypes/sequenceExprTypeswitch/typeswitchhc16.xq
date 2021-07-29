(: Name: typeswitchhq16 :)
(: Description: Simple test for typeswitch with operand expression, that is itself a typeswitch expression evaluating to integer type :)

typeswitch(typeswitch (1) case $i as xs:integer return $i default return <a>fn:false</a> )
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:integer
    return <wrap>test passed - the operand expression should evaluate to an integer type</wrap>
 default
   return <wrap>test failed</wrap>
