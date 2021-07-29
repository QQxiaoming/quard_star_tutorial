declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-1 :)
(: description : Evaluation of a validate expression with improper operand for operation.:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

validate {$input-context1/DataValues/Strings union $input-context1/DataValues/PositiveNumbers}