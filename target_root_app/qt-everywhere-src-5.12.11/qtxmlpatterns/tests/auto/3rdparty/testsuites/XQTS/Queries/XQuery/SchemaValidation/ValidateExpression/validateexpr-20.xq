declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-20 :)
(: description : Evaluation of a validate expression that a newly created lax element node used with a typeswitch expression. :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

 validate lax {typeswitch ("A String")
                case $i as xs:decimal
                 return <wrap>test failed</wrap> 
                case $i as xs:string
                 return <Strings><orderData>A String</orderData></Strings> 
                default 
                 return <Failed>This test failed</Failed> }