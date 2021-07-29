(: Name: ancestorself-20 :)
(: Description: Evaluation of the ancestor-or-self axis that used as part of the deep-equal-function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:deep-equal($input-context1/works/employee[12]/overtime/ancestor-or-self::works,$input-context1/works/employee[12]/overtime/ancestor-or-self::works)