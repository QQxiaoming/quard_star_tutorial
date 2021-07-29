(:*******************************************************:)
(:Test: op-dateTime-less-than-10                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-less-than" function used  :)
(:together with "or" expression (le operator).           :)
(:*******************************************************:)
 
(xs:dateTime("2002-04-03T12:00:10Z") le xs:dateTime("1990-04-02T12:10:00Z")) or (xs:dateTime("1975-04-03T12:10:00Z") le xs:dateTime("2000-02-02T12:00:09Z"))