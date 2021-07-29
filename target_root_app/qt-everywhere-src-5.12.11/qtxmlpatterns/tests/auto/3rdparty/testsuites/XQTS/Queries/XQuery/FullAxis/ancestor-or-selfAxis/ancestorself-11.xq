(: Name: ancestorself-11 :)
(: Description: Evaluation of the ancestor-or-self axis that is part of an "union " operation. Both operands are the same. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/*/day/ancestor-or-self::overtime) | ($input-context1/works/employee[12]/*/day/ancestor-or-self::overtime)