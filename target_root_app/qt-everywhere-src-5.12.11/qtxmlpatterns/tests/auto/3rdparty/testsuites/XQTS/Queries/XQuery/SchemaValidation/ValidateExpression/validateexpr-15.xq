declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-15 :)
(: description : Evaluation of a validate expression that a newly created lax element node used with an "if" expression. :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

 validate lax {if (fn:true()) then <Strings><orderData>A String</orderData></Strings> else <Failed>This test failed</Failed>}
