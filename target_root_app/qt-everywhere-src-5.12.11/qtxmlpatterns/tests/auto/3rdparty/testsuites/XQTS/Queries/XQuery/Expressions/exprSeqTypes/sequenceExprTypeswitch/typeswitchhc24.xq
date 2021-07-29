(: Name: typeswitchhq24 :)
(: Description: Simple test for evaluation of atomic value ("1") and return clause containing a division (div) operation :)
typeswitch (1)
 case $i as xs:double
    return <wrap>test failed</wrap> 
 case $i as xs:integer
   return 10 div 2
 case $i as xs:string
    return <wrap>test failed</wrap>
 default
   return <wrap>test failed</wrap>