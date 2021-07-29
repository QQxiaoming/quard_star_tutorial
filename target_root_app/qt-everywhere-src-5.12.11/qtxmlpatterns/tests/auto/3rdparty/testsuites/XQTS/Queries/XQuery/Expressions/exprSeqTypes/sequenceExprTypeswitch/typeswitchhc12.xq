(: Name: typeswitchhq12 :)
(: Description: Simple test for evaluating dynamic error raised by default clause (no error raised) :)

typeswitch(123)
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:integer
    return <wrap>test passed - If a dynamic error is generated, then test failed.</wrap>
 default
   return 12 div 0
