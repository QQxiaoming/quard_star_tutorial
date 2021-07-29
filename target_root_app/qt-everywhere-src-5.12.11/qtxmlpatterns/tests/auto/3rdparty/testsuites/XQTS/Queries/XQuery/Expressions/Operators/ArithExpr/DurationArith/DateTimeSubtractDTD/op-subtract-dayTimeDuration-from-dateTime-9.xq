(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-dateTime-9      :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of the "subtract-dayTimeDuration-from-dateTime" :)
(:operator used together with an "and" expression.       :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1993-12-09T13:13:13Z") - xs:dayTimeDuration("P03DT01H04M"))) and fn:string((xs:dateTime("1993-12-09T13:13:13Z") - xs:dayTimeDuration("P01DT01H03M")))