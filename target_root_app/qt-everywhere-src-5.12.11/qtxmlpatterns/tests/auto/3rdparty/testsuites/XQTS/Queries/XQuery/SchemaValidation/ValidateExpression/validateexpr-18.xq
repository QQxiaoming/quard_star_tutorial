declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-18 :)
(: description : Evaluation of a validate expression that a newly created lax element node used with a quantified expression ("every" operand). :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

 validate lax {if (every $x in (1,2,3) satisfies $x < 5) then <Strings><orderData>A String</orderData></Strings> else <Failed>This test failed</Failed> }