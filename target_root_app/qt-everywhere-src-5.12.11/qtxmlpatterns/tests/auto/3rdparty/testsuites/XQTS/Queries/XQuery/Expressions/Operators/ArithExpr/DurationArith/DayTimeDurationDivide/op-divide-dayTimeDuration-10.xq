(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-10                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" function used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P05DT09H02M")  div 2.0)) or fn:string((xs:dayTimeDuration("P05DT05H03M")  div 2.0))