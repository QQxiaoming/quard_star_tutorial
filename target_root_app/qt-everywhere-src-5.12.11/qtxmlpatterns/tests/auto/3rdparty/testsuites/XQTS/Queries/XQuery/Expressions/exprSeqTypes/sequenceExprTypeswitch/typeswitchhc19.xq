(: Name: typeswitchhq19 :)
(: Description: Simple test for evaluation of atomic value (double) and return clause containing a double addition operation. :)

typeswitch (5.1E2)
 case $i as xs:integer
   return <wrap>test failed2</wrap> 
 case $i as xs:double
    return xs:double(5.1E2 + 1.1E2)
 default
   return <wrap>test failed</wrap>