(:*******************************************************:)
(:Test: nodeexpressionhc4.xq                             :)  
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 7, 2005                                     :)
(:Purpose: Test of a Node Expression used as part of a   :)
(:boolean greater than expression (gt) operator.         :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context2 external;
(: insert-end :)

(($input-context2/staff[1]/employee[1]/empnum[1]) is ($input-context2/staff[1]/employee[1]/empnum[1])) gt (($input-context2/staff[1]/employee[1]/empnum[1]) is ($input-context2/staff[1]/employee[1]/empnum[1]))
