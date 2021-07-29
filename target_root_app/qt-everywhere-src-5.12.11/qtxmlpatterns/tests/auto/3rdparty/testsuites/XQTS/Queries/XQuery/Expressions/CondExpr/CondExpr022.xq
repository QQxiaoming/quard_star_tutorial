(:*******************************************************:)
(:Name: CondExpr022                                      :)
(:Written By: Nicolae Brinza                             :)
(:Purpose: Test case where then-expression raises        :)
(: a dynamic error, but test-expression selects          :)
(: else-expression so no error is raised                 :)
(:*******************************************************:)

if (doc-available('nodocument.xml')) 
then doc('nodocument.xml')
else 10 cast as xs:double
