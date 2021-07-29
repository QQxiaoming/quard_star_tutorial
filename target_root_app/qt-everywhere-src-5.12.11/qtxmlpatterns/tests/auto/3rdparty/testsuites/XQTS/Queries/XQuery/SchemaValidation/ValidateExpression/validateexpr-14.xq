declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-14 :)
(: description : Evaluation of a validate expression that a newly created strict element node used with an "if" expression. :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

 validate strict {if (fn:true()) then <Strings><orderData>A String</orderData></Strings> else <Failed>This test failed</Failed>}