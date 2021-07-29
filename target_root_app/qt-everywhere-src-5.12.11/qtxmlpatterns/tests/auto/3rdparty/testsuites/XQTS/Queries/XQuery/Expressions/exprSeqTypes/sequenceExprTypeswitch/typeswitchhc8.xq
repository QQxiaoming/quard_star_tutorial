(: Name: typeswitchhq8 :)
(: Description: Simple test for evaluation of simple time expression and various data types for case evaluation :)

typeswitch(xs:time("12:00:00"))
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:integer
    return <wrap>test failed</wrap> 
 case $i as xs:time
    return <wrap>test passed - xs:time("12:00:00")is of time type</wrap>
 default
   return <wrap>test failed</wrap>