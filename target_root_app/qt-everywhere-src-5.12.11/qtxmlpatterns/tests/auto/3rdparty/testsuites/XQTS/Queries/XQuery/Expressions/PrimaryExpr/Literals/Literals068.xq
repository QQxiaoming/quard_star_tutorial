(: Name: Literals068 :)
(: Description: Test the escaping of the &apos; (apostrophe) and &quot; (quotation) characters as part of an XML text node constructor :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

text{ 'He said, "I don''t like it."' }