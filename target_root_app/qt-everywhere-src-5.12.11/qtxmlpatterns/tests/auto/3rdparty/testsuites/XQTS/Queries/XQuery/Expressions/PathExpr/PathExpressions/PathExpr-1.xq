(:*******************************************************:)
(:Test: PathExpr-1                                       :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Leading lone slash syntax contraints          :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context[5 * /])
