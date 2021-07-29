(: Name: fn-idref-dtd-24 :)
(: Description: Evaluation of fn:idref, where an IDREFS node contains multiple IDREFs. For errata FO.E29. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref("language", $input-context1)}</results>
