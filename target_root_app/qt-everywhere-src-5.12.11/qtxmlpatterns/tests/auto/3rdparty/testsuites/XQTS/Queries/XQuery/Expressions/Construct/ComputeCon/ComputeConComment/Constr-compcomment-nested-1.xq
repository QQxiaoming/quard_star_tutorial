(: Name: Constr-compcomment-nested-1 :)
(: Written by: Andreas Behm :)
(: Description: nested computed comment node constructor :)

comment {comment {'one', comment {'two'}}, 'three', comment {'four'}}
