declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-6 :)
(: description : Evaluation of a validate expression that results on a PI node with strict mode.:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

validate strict {<?format role="output" ?>}