(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-2             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "add-dayTimeDuration-to-dateTime" operator :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string((xs:dateTime("2000-12-12T11:10:03Z") + xs:dayTimeDuration("P12DT10H07M"))) and fn:false()