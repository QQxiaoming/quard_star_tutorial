(:*******************************************************:)
(:Test: op-boolean-equal-16                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 24, 2005                                    :)
(:Purpose: Evaluates The "boolean-equal" function        :)
(: using the "ends-with" function.                       :)
(:*******************************************************:)
 
fn:ends-with("Query","ry") eq  xs:boolean("false")