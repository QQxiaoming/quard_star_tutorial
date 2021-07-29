(: Name: Literals066 :)
(: Description: Test the escaping of the ' (apostrophe) and " (quotation) characters as part of an XML element constructor :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<test>{ 'He said, "I don''t like it."' }</test>