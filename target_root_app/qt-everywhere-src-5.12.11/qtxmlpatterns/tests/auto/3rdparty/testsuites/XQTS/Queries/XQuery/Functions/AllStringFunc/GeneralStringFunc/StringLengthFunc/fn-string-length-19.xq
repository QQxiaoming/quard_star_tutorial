(:*******************************************************:)
(: Name: fn-string-length-19                             :)
(: Written By: Nicolae Brinza                            :)
(: Description: Evaluation of string-length function with:)
(: an argument that is a sequence of more than one item  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:string-length( $input-context//location )
