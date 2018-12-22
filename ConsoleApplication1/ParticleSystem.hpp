class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:

	ParticleSystem(unsigned int count) :
		m_particles(count),
		m_vertices(sf::Triangles, count * 3),
		m_lifetime(sf::seconds(10.f)),
		m_emitter(0.f, 0.f),
		intensity_(0),
		cur_intens_(0)
	{
	}

	void setIntensity(int intensity)
	{
		intensity_ += intensity;
	}

	void setEmitter(sf::Vector2f position)
	{
		m_emitter = position;
	}

	void update(sf::Time elapsed)
	{
		for (std::size_t i = 0; i < m_particles.size(); ++i)
		{
			// update the particle lifetime
			Particle& p = m_particles[i];
			p.lifetime -= elapsed;

			// if the particle is dead, respawn it
			if (p.lifetime <= sf::Time::Zero)
				resetParticle(i);

			// update the position of the corresponding vertex
			m_vertices[i*3].position += p.velocity * elapsed.asSeconds();
			m_vertices[i*3 + 1].position += p.velocity * elapsed.asSeconds();
			m_vertices[i*3 + 2].position += p.velocity * elapsed.asSeconds();

			// update the alpha (transparency) of the particle according to its lifetime
			float ratio = p.lifetime.asSeconds() / m_lifetime.asSeconds();
			//m_vertices[i].color.a = static_cast<sf::Uint8>(ratio * 255);
			m_vertices[i*3].color = sf::Color::Blue;
			m_vertices[i*3 + 1].color = sf::Color::Blue;
			m_vertices[i*3 + 2].color = sf::Color::Blue;

			if (!m_particles[i].visible)
			{
				m_vertices[i*3].color.a = static_cast<sf::Uint8>(0);
				m_vertices[i*3 + 1].color.a = static_cast<sf::Uint8>(0);
				m_vertices[i*3 + 2].color.a = static_cast<sf::Uint8>(0);
			}
		}
	}

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		// apply the transform
		states.transform *= getTransform();

		// our particles don't use a texture
		states.texture = NULL;

		// draw the vertex array
		target.draw(m_vertices, states);
	}

private:

	struct Particle
	{
		sf::Vector2f velocity;
		sf::Time lifetime;
		bool visible;
	};

	void resetParticle(std::size_t index)
	{
		if(cur_intens_ > 0) cur_intens_--;
		// give a random velocity and lifetime to the particle
		float angle = (std::rand() % 360) * 3.14f / 180.f;
		float speed = (std::rand() % 200) + 50.f;
		m_particles[index].velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
		m_particles[index].lifetime = sf::milliseconds((std::rand() % 5000) + 1000);

		// reset the position of the corresponding vertex
		m_vertices[index*3].position = m_emitter;
		m_vertices[index*3 + 1].position = m_emitter + sf::Vector2f(std::rand()%5, 0.0);
		m_vertices[index*3 + 2].position = m_emitter +sf::Vector2f(0.0, std::rand()%5);

		//m_vertices[index].color = sf::Color::Blue;
		if (intensity_ > cur_intens_)
		{
			m_particles[index].visible = true;
			cur_intens_++;
			if(intensity_ > 0)intensity_--;
		}
		else {
			m_particles[index].visible = false;
		}
	}

	std::vector<Particle> m_particles;
	sf::VertexArray m_vertices;
	sf::Time m_lifetime;
	sf::Vector2f m_emitter;
	int intensity_;
	int cur_intens_;
}; 
