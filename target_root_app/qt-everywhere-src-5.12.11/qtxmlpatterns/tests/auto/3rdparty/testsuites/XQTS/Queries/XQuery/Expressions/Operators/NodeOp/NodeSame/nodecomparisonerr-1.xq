(:Test: nodecomparisonerr-1                              :)  
(:Description: A node comparison where one of the operands is not the:)
(:empty sequence or a single node.                       :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(() is 100)
