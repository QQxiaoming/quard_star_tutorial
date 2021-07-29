(:*******************************************************:)
(:Test: nodeexpressionhc2.xq                             :)  
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 7, 2005                                     :)
(:Purpose: Test of a Node Expression used as part        :)
(:of a boolean-less-than expression (lt operator).       :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context2 external;
(: insert-end :)

(($input-context2/staff[1]/employee[1]/empnum[1]) is ($input-context2/staff[1]/employee[1]/empnum[1])) lt (($input-context2/staff[1]/employee[1]/empnum[1]) is ($input-context2/staff[1]/employee[1]/empnum[1]))
