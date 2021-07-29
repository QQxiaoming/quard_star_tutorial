declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-7 :)
(: description : Evaluation of a validate expression that results on a PI node with lax mode.:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

validate lax {<?format role="output" ?>}