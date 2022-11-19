SELECT id , ShipCountry , 
			CASE WHEN  ShipCountry == 'USA' 
							OR ShipCountry == 'Mexico' 
							OR ShipCountry == 'Canada' 
			THEN 'NorthAmerica' 
			ELSE 'OtherPlace' 
			END
FROM 'Order' 
WHERE id >= 15445
ORDER BY id ASC
LIMIT 20;


