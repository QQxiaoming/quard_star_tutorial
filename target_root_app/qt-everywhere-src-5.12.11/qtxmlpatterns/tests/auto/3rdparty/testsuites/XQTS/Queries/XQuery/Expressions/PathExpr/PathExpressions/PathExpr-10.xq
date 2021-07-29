(:*******************************************************:)
(:Test: PathExpr-10                                      :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Leading lone slash syntax contraints          :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context[if ($doclevel) then / else /*])
