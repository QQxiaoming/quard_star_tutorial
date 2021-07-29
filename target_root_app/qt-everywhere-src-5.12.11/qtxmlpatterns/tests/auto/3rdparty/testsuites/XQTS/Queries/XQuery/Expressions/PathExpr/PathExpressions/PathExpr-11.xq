(:*******************************************************:)
(:Test: PathExpr-11                                      :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Leading lone slash syntax contraints          :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare variable $a := $input-context;

fn:count($input-context[/ is $a])
