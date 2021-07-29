(:*******************************************************:)
(:Test: PathExpr-12                                      :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Leading lone slash syntax contraints          :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context[/ instance of document-node(schema-element(x))])
