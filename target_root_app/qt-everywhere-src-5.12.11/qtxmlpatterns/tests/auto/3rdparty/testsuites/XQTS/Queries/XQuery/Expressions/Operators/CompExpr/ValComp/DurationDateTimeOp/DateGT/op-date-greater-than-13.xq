(:*******************************************************:)
(:Test: op-date-greater-than-13                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used:)
(:together with "fn:false"/or expression (gt operator).  :)
(:*******************************************************:)
 
(xs:date("1980-01-01Z") gt xs:date("1980-10-01Z")) or (fn:false())