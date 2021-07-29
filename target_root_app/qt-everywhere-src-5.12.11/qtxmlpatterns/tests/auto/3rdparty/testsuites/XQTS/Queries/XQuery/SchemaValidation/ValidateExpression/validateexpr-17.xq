declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-17 :)
(: description : Evaluation of a validate expression that a newly created lax element node used with an "or" expression :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

 validate lax {if (fn:true() or fn:true()) then <Strings><orderData>A String</orderData></Strings> else <Failed>This test failed</Failed> }
