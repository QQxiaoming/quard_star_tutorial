(: Name: typeswitchhq14 :)
(: Description: Simple test for typeswitch with operand expression (boolean) evaluating to boolean type :)

typeswitch(fn:true() and fn:true())
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:boolean
    return <wrap>test passed - "fn:true() and fn:true()" should evaluate to boolean type</wrap>
 default
   return <wrap>test failed</wrap>
