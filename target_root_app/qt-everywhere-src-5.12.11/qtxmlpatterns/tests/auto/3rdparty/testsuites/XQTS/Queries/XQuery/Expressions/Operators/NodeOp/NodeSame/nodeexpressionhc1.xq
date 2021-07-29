(:*******************************************************:)
(:Test: nodeexpressionhc1.xq                             :) 
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 7, 2005                                     :)
(:Purpose: Test of a Node Expression used as an argument :)
(:to the fn not function                                 :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context2 external;
(: insert-end :)

fn:not(($input-context2/staff[1]/employee[1]/empnum[1]) is ($input-context2/staff[1]/employee[1]/empnum[1]))
