(: Name: typeswitchhq11 :)
(: Description: Simple test for evaluation of various expressions and should mathc the deafult value :)

typeswitch(123)
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:anyURI
    return <wrap>test failed</wrap>
 default
   return <wrap>test passed - 123 is an integer (not an option on any cases)</wrap>