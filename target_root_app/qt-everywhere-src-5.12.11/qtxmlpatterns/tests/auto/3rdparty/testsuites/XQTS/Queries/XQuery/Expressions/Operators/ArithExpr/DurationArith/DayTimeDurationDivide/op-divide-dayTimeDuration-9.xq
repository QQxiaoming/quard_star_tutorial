(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-9                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" function used :)
(:together with and "and" expression.                    :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P01DT02H01M") div 2.0)) and fn:string((xs:dayTimeDuration("P02DT03H03M") div 2.0 ))