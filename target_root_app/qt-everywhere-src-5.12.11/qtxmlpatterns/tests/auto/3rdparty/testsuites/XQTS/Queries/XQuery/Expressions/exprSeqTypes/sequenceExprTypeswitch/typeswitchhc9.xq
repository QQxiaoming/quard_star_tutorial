(: Name: typeswitchhq9 :)
(: Description: Simple test for evaluation of simple dateTime expression and various data types for case evaluation :)

typeswitch(xs:dateTime("1999-12-31T19:20:00"))
 case $i as xs:string
    return <wrap>test failed</wrap> 
 case $i as xs:integer
    return <wrap>test failed</wrap> 
 case $i as xs:dateTime
    return <wrap>test passed - xs:dateTime("1999-12-31T19:20:00")is of dateTime type</wrap>
 default
   return <wrap>test failed</wrap>