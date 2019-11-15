<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" omit-xml-declaration="yes" indent="yes"/>
	
	<xsl:template match="@* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/element/tag[@type = 'string']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<string><xsl:value-of select="text()"/></string>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/fallback-element/tag[@type = 'string']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<string><xsl:value-of select="text()"/></string>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/element/tag[@type = 'single precision real']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<single-precision-real><xsl:value-of select="text()"/></single-precision-real>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/fallback-element/tag[@type = 'single precision real']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<single-precision-real><xsl:value-of select="text()"/></single-precision-real>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/element/tag[@type = 'unsigned integer 32bit']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<unsigned-integer-32bit><xsl:value-of select="text()"/></unsigned-integer-32bit>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/fallback-element/tag[@type = 'unsigned integer 32bit']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<unsigned-integer-32bit><xsl:value-of select="text()"/></unsigned-integer-32bit>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/element/tag[@type = 'unsigned integer 8bit']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<unsigned-integer-8bit><xsl:value-of select="text()"/></unsigned-integer-8bit>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/fallback-element/tag[@type = 'unsigned integer 8bit']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<unsigned-integer-8bit><xsl:value-of select="text()"/></unsigned-integer-8bit>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/element/tag[@type = 'boolean']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<boolean><xsl:value-of select="text()"/></boolean>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/fallback-element/tag[@type = 'boolean']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
			<boolean><xsl:value-of select="text()"/></boolean>
			<xsl:text disable-output-escaping="yes">&#xa;&#x9;&#x9;&#x9;&#x9;&#x9;&#x9;</xsl:text>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/element/tag[@type = 'nothing']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
		</tag>
	</xsl:template>
	
	<xsl:template match="/getter/*/interpretation/apply-enumeration/enumeration/fallback-element/tag[@type = 'nothing']">
		<tag>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
		</tag>
	</xsl:template>
</xsl:stylesheet>
