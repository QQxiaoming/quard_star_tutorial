declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-21 :)
(: description : Evaluation of a validate expression that a newly created lax element node used with a FLWOR expression. :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

 validate lax {let $var := <Strings><orderData>A String</orderData></Strings> 
               return 
                 $var }