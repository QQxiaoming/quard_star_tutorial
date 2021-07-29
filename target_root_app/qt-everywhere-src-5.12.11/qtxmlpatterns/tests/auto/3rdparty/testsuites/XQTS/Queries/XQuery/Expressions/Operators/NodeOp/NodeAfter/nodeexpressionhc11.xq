(:*******************************************************:)
(:test nodeexpressionhc11                                :)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 8, 2005                                     :)
(:Purpose: Test of a Node after expression used as an    :)
(:argument to an "fn:not" function.                      :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context2 external;
(: insert-end :)

fn:not(($input-context2/staff[1]/employee[1]/empnum[1]) >> ($input-context2/staff[1]/employee[1]/empnum[1]))
