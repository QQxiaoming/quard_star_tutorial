(:*******************************************************:)
(:Test: op-date-greater-than-14                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used:)
(:together with "fn:false"/or expression (ge operator).  :)
(:*******************************************************:)
 
(xs:date("1980-10-25Z") ge xs:date("1980-10-26Z")) or (fn:false())