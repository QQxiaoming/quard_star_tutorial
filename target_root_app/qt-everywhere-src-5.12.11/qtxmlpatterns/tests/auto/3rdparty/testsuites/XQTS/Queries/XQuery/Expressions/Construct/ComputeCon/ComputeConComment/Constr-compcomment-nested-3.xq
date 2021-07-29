(: Name: Constr-compcomment-nested-3 :)
(: Written by: Andreas Behm :)
(: Description: nested computed comment nodes in element constructor :)

document {comment {'one'}, <a/>, comment {'two'}, <b/>, comment {'three'}}
