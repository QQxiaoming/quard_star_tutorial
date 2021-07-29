(: Name: typeswitchhq10 :)
(: Description: Simple test for evaluation of simple anyURI expression and various data types for case evaluation :)

typeswitch(xs:anyURI("http://example.com"))
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:integer
    return <wrap>test failed</wrap> 
 case $i as xs:anyURI
    return <wrap>test passed - xs:anyURI("http://www.example.com")is of anyURI type</wrap>
 default
   return <wrap>test failed</wrap>