(: Name: typeswitchhq15 :)
(: Description: Simple test for typeswitch with operand expression (boolean or boolean) evaluating to boolean type :)

typeswitch(fn:true() or fn:false())
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:boolean
    return <wrap>test passed - "fn:true() or fn:false()" should evaluate to boolean type</wrap>
 default
   return <wrap>test failed</wrap>
