(:*******************************************************:)
(:test nodeexpression14hc                                :)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 8, 2005                                     :)
(:Purpose: Test of a node after expression used as part  :)
(:of a boolean-greater-than expression (gt operator).    :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context2 external;
(: insert-end :)

(($input-context2/staff[1]/employee[1]/empnum[1]) >> ($input-context2/staff[1]/employee[1]/empnum[1])) gt (($input-context2/staff[1]/employee[1]/empnum[1]) >> ($input-context2/staff[1]/employee[1]/empnum[1]))
